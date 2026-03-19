import os
import sys
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns
import pickle
import warnings

from sklearn.ensemble import RandomForestRegressor
from sklearn.multioutput import MultiOutputRegressor
from sklearn.model_selection import StratifiedKFold, train_test_split
from sklearn.preprocessing import StandardScaler
from sklearn.metrics import mean_squared_error, mean_absolute_error, r2_score
from imblearn.over_sampling import RandomOverSampler  # Simple oversampling

# Ignore convergence or future warnings to clean up the output
warnings.filterwarnings("ignore")

# 2. GLOBAL CONFIGURATIONS AND HYPERPARAMETERS

# Experiment Definitions
N_ALGORITMOS = 3
OVERSAMPLE = False
IS_MAXIMIZATION = True 

# Random Forest Hyperparameters
RF_PARAMS = {
    'n_estimators': 1000,
    'max_depth': 5,
    'min_samples_split': 2,
    'min_samples_leaf': 3,
    'random_state': 42,
    'n_jobs': -2              
}

# 3. AUXILIARY FUNCTIONS (Metrics and Tiebreak)

def identify_best_with_tiebreak(row, epsilon=1e-9, seed=42, is_maximization=True):
    """
    Identifies the best algorithm (highest or lowest value) in a row (instance),
    with random tie-breaking for identical values.
    """
    if is_maximization:
        best_value = row.max()
        candidates = row[row == best_value]
    else:
        best_value = row.min()
        candidates = row[row == best_value]

    if len(candidates) == 1:
        return candidates.index[0]

    np.random.seed(seed + abs(hash(tuple(row.values))) % 100000)
    noise = np.random.uniform(0, epsilon, len(candidates))

    if is_maximization:
        tiebreak_values = candidates.values + noise # Add noise to break ties; the one with higher + noise is best
        winner_idx = np.argmax(tiebreak_values)
    else:
        tiebreak_values = candidates.values - noise # Subtract noise to break ties; the one with lower - noise is best
        winner_idx = np.argmin(tiebreak_values)

    return candidates.index[winner_idx]

def calculate_robust_mape(y_true, y_pred, epsilon=1e-10):
    """
    Calculates MAPE handling division by zero.
    """
    y_true, y_pred = np.array(y_true), np.array(y_pred)
    valid_indices = np.abs(y_true) > epsilon
    if not valid_indices.any():
        return np.nan
    return np.mean(
        np.abs((y_true[valid_indices] - y_pred[valid_indices]) /
              np.maximum(np.abs(y_true[valid_indices]), epsilon))
    ) * 100


def calculate_robust_r2(y_true, y_pred, epsilon=1e-10):
    """
    Calculates R2 handling zero variance.
    """
    if np.var(y_true) < epsilon:
        return np.nan # Zero variance, R2 undefined
    return r2_score(y_true, y_pred)

def normalize_importance(values):
    """
    Normalizes importances between 0 and 1.
    """
    if np.max(values) == np.min(values):
        return values
    return (values - np.min(values)) / (np.max(values) - np.min(values))

# 4. DATASET PROCESSING (Training and Merit)

def process_dataset(csv_path, label, l_val, r_val, walk_type, n_algoritmos, oversample, is_maximization, result_base_path, rf_params):
    """
    Loads dataset, trains model, saves artifacts, and calculates merit.
    """
    dataset_name = f"{walk_type}_l{l_val}_r{r_val}" if walk_type == 'random_walk' else f"{walk_type}_r{r_val}"


    # 1. Load Data
    df = pd.read_csv(csv_path)

    # Drop specified columns if they exist
    columns_to_drop_specifically = ['NSGA2_HV', 'NSGA2_IGD']
    df = df.drop(columns=[col for col in columns_to_drop_specifically if col in df.columns], errors='ignore')

    # Drop column based on experiment label, if it exists
    coluna = f"{label}_{label}" # Use the passed 'label' parameter
    df = df.drop(columns=[coluna], errors='ignore')

    # Identify feature and target columns
    # The last 'n_algoritmos' columns are the targets (algorithms)
    target_cols = df.columns[-n_algoritmos:]
    # Features are all columns except the first (Instance) and the last (Targets)
    # Removing column 0 because it's the instance number
    feature_cols = df.columns[1:-n_algoritmos]

    X = df[feature_cols]
    y = df[target_cols]
    instances = df.iloc[:, 0]

    # 2. Identify Winners
    # For each instance, who has the HIGHEST value (for maximization) or LOWEST value (for minimization)?
    best_algorithm_series = y.apply(identify_best_with_tiebreak, axis=1, is_maximization=is_maximization)
    winning_algorithms = set(best_algorithm_series.unique())

    # 3. Configure Cross-Validation (10-Fold Stratified)
    kf = StratifiedKFold(n_splits=10, shuffle=True, random_state=42)

    # Store fold results
    fold_merits = []
    as_preds_all = []
    vbs_preds_all = []
    feature_importances_list = []

    # If Stratified fails (e.g., classes with 1 member), do normal KFold
    try:
        splits = list(kf.split(X, best_algorithm_series))
    except ValueError:
        from sklearn.model_selection import KFold
        kf_sh = KFold(n_splits=10, shuffle=True, random_state=42)
        splits = list(kf_sh.split(X))

    # Fold Loop
    for fold_idx, (train_idx, test_idx) in enumerate(splits):
        X_train, X_test = X.iloc[train_idx], X.iloc[test_idx]
        y_train, y_test = y.iloc[train_idx], y.iloc[test_idx]
        train_winners = y_train.apply(identify_best_with_tiebreak, axis=1, is_maximization=is_maximization).values

        # 4. "Class" Balancing (RandomOverSampler)
        if oversample:
          counts = pd.Series(train_winners).value_counts()
          max_count = counts.max()

          # Only balance actual minority classes
          minority_classes = {
              cls: max_count
              for cls, c in counts.items()
              if c < max_count and cls in winning_algorithms
          }

          if len(minority_classes) > 0:
              ros = RandomOverSampler(
                  random_state=42,
                  sampling_strategy=minority_classes
              )

              train_indices = np.arange(len(X_train))
              resampled_indices, _ = ros.fit_resample(
                  train_indices.reshape(-1, 1),
                  train_winners
              )
              resampled_indices = resampled_indices.flatten()

              X_train = X_train.iloc[resampled_indices].reset_index(drop=True)
              y_train = y_train.iloc[resampled_indices].reset_index(drop=True)

        # 5. Normalization
        scaler = StandardScaler()
        X_train_scaled = scaler.fit_transform(X_train)
        X_test_scaled = scaler.transform(X_test)

        # 6. Training (MultiOutput RandomForest)
        model = MultiOutputRegressor(RandomForestRegressor(**rf_params))
        model.fit(X_train_scaled, y_train)

        # 7. Prediction and Fold Metrics
        y_pred = pd.DataFrame(model.predict(X_test_scaled), columns=y.columns, index=y_test.index)

        # Feature importance (average of estimators)
        feat_imp = np.mean([est.feature_importances_ for est in model.estimators_], axis=0)
        feature_importances_list.append(feat_imp)

        # Determine AS (Algorithm Selection - Predicted)
        if is_maximization:
            # Maximization: AS is the algorithm with the highest predicted value
            as_choice = y_pred.idxmax(axis=1)
        else:
            # Minimization: AS is the algorithm with the lowest predicted value
            as_choice = y_pred.idxmin(axis=1)

        # Real values corresponding to the choices
        val_AS = []
        val_VBS = []
        val_SBS = [] # Single Best Solver from Training

        # Calculate SBS from training (algorithm with highest/lowest average in training)
        if is_maximization:
            sbs_alg = y_train.mean().idxmax()
        else:
            sbs_alg = y_train.mean().idxmin()

        # VBS from test (Ground Truth)
        vbs_choice = y_test.apply(identify_best_with_tiebreak, axis=1, is_maximization=is_maximization)

        for idx_test in y_test.index:
            # Real value of the algorithm chosen by the model
            val_AS.append(y_test.loc[idx_test, as_choice[idx_test]])
            # Real value of VBS
            val_VBS.append(y_test.loc[idx_test, vbs_choice[idx_test]])
            # Real value of SBS
            val_SBS.append(y_test.loc[idx_test, sbs_alg])

        # Calculate Fold Merit
        # (Avg_AS - Avg_VBS) / (Avg_SBS - Avg_VBS)
        mean_as = np.mean(val_AS)
        mean_vbs = np.mean(val_VBS)
        mean_sbs = np.mean(val_SBS)

        # Avoid division by zero if SBS == VBS
        denominator = (mean_sbs - mean_vbs)
        if abs(denominator) < 1e-9:
            merit = np.nan
        elif is_maximization:
            merit = (mean_vbs - mean_as) / denominator
        else:
            merit = (mean_as - mean_vbs) / denominator
        fold_merits.append(merit)

        # Store predictions for later analysis
        as_preds_all.extend(list(zip(test_idx, val_AS))) # (original_index, value)
        vbs_preds_all.extend(list(zip(test_idx, val_VBS)))

    # 8. Consolidate Dataset Results
    final_merit = np.nanmean(fold_merits)
    avg_feat_importance = np.mean(feature_importances_list, axis=0)

    # 9. Save Artifacts
    # name
    nome_arquivo = f"{dataset_name}"

    # Feature Importance CSV
    fi_df = pd.DataFrame({'Feature': feature_cols, 'Importance': avg_feat_importance})
    fi_df.to_csv(os.path.join(result_base_path, 'features_importance', f"{nome_arquivo}_features_importance.csv"), index=False)

    # Predictions (Reconstructing original order by indices)
    as_preds_df = pd.DataFrame(as_preds_all, columns=['Index', 'AS_Prediction']).set_index('Index').sort_index()
    vbs_preds_df = pd.DataFrame(vbs_preds_all, columns=['Index', 'VBS_Prediction']).set_index('Index').sort_index()

    # Save AS and VBS prediction CSVs
    pred_path_as = os.path.join(result_base_path, 'models', 'theoretical_models', nome_arquivo, 'AS')
    pred_path_vbs = os.path.join(result_base_path, 'models', 'theoretical_models', nome_arquivo, 'VBS')
    os.makedirs(pred_path_as, exist_ok=True)
    os.makedirs(pred_path_vbs, exist_ok=True)

    # Map back to instance names
    as_preds_df['Instance'] = instances
    vbs_preds_df['Instance'] = instances

    as_preds_df.to_csv(os.path.join(pred_path_as, f"{nome_arquivo}_ASModel.csv"), index=False)
    vbs_preds_df.to_csv(os.path.join(pred_path_vbs, f"{nome_arquivo}_VBSModel.csv"), index=False)

    print(f"   [OK] {dataset_name} | Merit: {final_merit:.4f}")

    return {
        'l': l_val,
        'r': r_val,
        'walk_type': walk_type,
        'merit': final_merit,
        'dataset_name': nome_arquivo
    }

# 5. MAIN ORCHESTRATOR

def run_pipeline(folder_path, experiment_label, base_output_path, is_maximization):
    print(">>> STARTING ALGORITHM SELECTION PIPELINE <<<")
    print(f"Data Source: {folder_path}")
    # Set LABEL dynamically for the current experiment
    LABEL = experiment_label
    current_result_base_path = os.path.join(base_output_path, experiment_label)

    print(f"Config: Label={LABEL}, Maximization={is_maximization}, Oversample={OVERSAMPLE})")
    print(f"Model: RF (Max Depth={RF_PARAMS['max_depth']})")

    # Recreate output folder structure for the current LABEL
    dirs = [
        current_result_base_path,
        os.path.join(current_result_base_path, 'models', 'pickle_models'),
        os.path.join(current_result_base_path, 'models', 'theoretical_models'),
        os.path.join(current_result_base_path, 'predictions'),
        os.path.join(current_result_base_path, 'figures'),
        os.path.join(current_result_base_path, 'regression_metrics'),
        os.path.join(current_result_base_path, 'features_importance')
    ]
    for d in dirs:
        os.makedirs(d, exist_ok=True)

    # Instruction 1: Rename l_values to raw_l_values
    raw_l_values = ['100', '50', '25', '10', '5']
    r_values = ['1.0', '0.5', '0.25', '0.1', '0.05']
    walk_types = ['random_walk', 'adaptative_first_improvement', 'adaptative_non_dominated']

    results = []

    # Control sets to avoid duplicate processing
    processed_afi = set()
    processed_and = set()

    # Instruction 2: Locate the loop `for l_val in raw_l_values:`
    for l_val in raw_l_values:
        for r_val in r_values:
            for walk_type in walk_types:

                # Naming and Redundancy Logic Definition
                if walk_type == 'random_walk':
                    # Changed 'l' to 'L' to match file names
                    filename = f"random_walk_L{l_val}_r{r_val}_final.csv"
                    process_now = True
                    # Instruction 3: Modify display_l assignment
                    display_l = f'l={l_val}'

                elif walk_type == 'adaptative_first_improvement':
                    filename = f"adaptative_first_improvement_r{r_val}_final.csv"
                    if r_val in processed_afi:
                        continue
                    process_now = True
                    processed_afi.add(r_val)
                    display_l = 'l=100'

                elif walk_type == 'adaptative_non_dominated':
                    filename = f"adaptative_non_dominated_r{r_val}_final.csv"
                    if r_val in processed_and:
                        continue
                    process_now = True
                    processed_and.add(r_val)
                    display_l = 'l=100'

                if process_now:
                    full_path = os.path.join(folder_path, f"{LABEL}_datasets", walk_type, "final", filename)

                    if os.path.exists(full_path):
                        res = process_dataset(
                            full_path, LABEL, display_l, r_val, walk_type,
                            N_ALGORITMOS, OVERSAMPLE, is_maximization, current_result_base_path, RF_PARAMS
                        )
                        results.append(res)
                    else:
                        print(f"   [WARNING] File not found: {full_path}")

    # 6. Save Consolidated Merit Table
    merit_df = pd.DataFrame(results)

    # Instruction 4: Create prefixed_l_values
    prefixed_l_values = [f'l={val}' for val in raw_l_values]

    # Filter into separate DataFrames
    random_walk_df = merit_df[merit_df['walk_type'] == 'random_walk']
    afi_df = merit_df[merit_df['walk_type'] == 'adaptative_first_improvement']
    and_df = merit_df[merit_df['walk_type'] == 'adaptative_non_dominated']

    # Create pivot table for random_walk_df
    random_walk_pivot = random_walk_df.pivot_table(index='r', columns='l', values='merit')
    # Instruction 5: Replace l_values with prefixed_l_values
    random_walk_pivot = random_walk_pivot[prefixed_l_values]

    # Create pivot table for afi_df
    afi_pivot = afi_df.pivot_table(index='r', values='merit')
    afi_pivot.rename(columns={'merit': 'adaptative_first_improvement'}, inplace=True)

    # Create pivot table for and_df
    and_pivot = and_df.pivot_table(index='r', values='merit')
    and_pivot.rename(columns={'merit': 'adaptative_non_dominated'}, inplace=True)

    # Concatenate the pivoted DataFrames horizontally
    combined_merit_df = pd.concat([random_walk_pivot, afi_pivot, and_pivot], axis=1)

    # Reindex to ensure r_values order
    combined_merit_df = combined_merit_df.reindex(r_values)

    merit_path = os.path.join(current_result_base_path, f"merit_{LABEL}.csv")
    combined_merit_df.to_csv(merit_path)
    print(f"\n>>> Merit table saved to: {merit_path}")

    # 7. POST-PROCESSING: Metrics and Figures

    print("\n>>> Generating Final Figures and Metrics <<<")

    # A. Find Best Dataset (Lowest Merit - 0 is ideal)
    best_row = merit_df.loc[merit_df['merit'].idxmin()]
    best_dataset_name = best_row['dataset_name']
    print(f"Best Configuration Found: {best_dataset_name} (Merit: {best_row['merit']:.4f})")

    # Auxiliary plotting function for Feature Importance (horizontal bar plot), for visualizing while in coding stage
    def plot_fi(ax, df, title):
        df['normalized'] = normalize_importance(df['Importance'])
        df_sorted = df.sort_values(by='normalized', ascending=False)
        # Using seaborn barplot for horizontal bars
        sns.barplot(x='normalized', y='Feature', data=df_sorted, ax=ax, palette='viridis')
        ax.set_title(title)
        ax.set_xlabel("Normalized Importance")
        ax.set_ylabel("Feature")
        ax.tick_params(axis='y', labelsize=8)

    # B. Generate Feature Importance Figures for EACH processed dataset
    print("\n>>> Generating Feature Importance Figures for each dataset <<<")
    for res in results:
        d_name = res['dataset_name']
        fi_csv_path = os.path.join(current_result_base_path, 'features_importance', f"{d_name}_features_importance.csv")
        try:
            fi_df = pd.read_csv(fi_csv_path)
            num_features = len(fi_df)
            fig_height = max(6, num_features * 0.4) # Minimum height 6, then 0.4 per feature
            fig, ax = plt.subplots(figsize=(10, fig_height))
            plot_fi(ax, fi_df, f"Feature Importance for {d_name}")
            plt.tight_layout()
            plt.savefig(os.path.join(current_result_base_path, 'figures', f"feature_importance_{d_name}.png"))
            plt.close(fig)
            print(f"   [OK] Feature importance plot generated for {d_name}.")
        except Exception as e:
            print(f"   [ERROR] While generating feature importance figure for {d_name}: {e}")

    # C. Consolidated Regression Metrics (MSE, MAE, R2)
    metrics_list = []
    for res in results:
        d_name = res['dataset_name']
        try:
            as_df = pd.read_csv(os.path.join(current_result_base_path, 'models', 'theoretical_models', d_name, 'AS', f"{d_name}_ASModel.csv"))
            vbs_df = pd.read_csv(os.path.join(current_result_base_path, 'models', 'theoretical_models', d_name, 'VBS', f"{d_name}_VBSModel.csv"))

            merged = pd.merge(as_df, vbs_df, on='Instance')
            y_true = merged['VBS_Prediction']
            y_pred = merged['AS_Prediction']

            metrics_list.append({
                'dataset': d_name,
                'MSE': mean_squared_error(y_true, y_pred),
                'MAE': mean_absolute_error(y_true, y_pred),
                'R2': calculate_robust_r2(y_true, y_pred),
                'MAPE': calculate_robust_mape(y_true, y_pred)
            })
        except Exception as e:
            print(f"   [ERROR] While calculating regression metrics for {d_name}: {e}")
            pass

    # Start of new logic for saving regression metrics in pivoted format
    # Convert metrics_list to DataFrame
    all_metrics_df = pd.DataFrame(metrics_list)

    # Create a temporary df from results to get 'l', 'r', 'walk_type' for each 'dataset_name'
    dataset_info_df = pd.DataFrame(results)[['dataset_name', 'l', 'r', 'walk_type']]
    # Rename 'dataset_name' to 'dataset' to match all_metrics_df for merging
    dataset_info_df.rename(columns={'dataset_name': 'dataset'}, inplace=True)

    combined_all_metrics_df = pd.merge(all_metrics_df, dataset_info_df, on='dataset')

    # Define the list of metrics to process
    regression_metrics_to_save = ['MSE', 'MAE', 'R2', 'MAPE']

    for metric_name in regression_metrics_to_save:
        # Filter into separate DataFrames for current metric
        metric_random_walk_df = combined_all_metrics_df[
            combined_all_metrics_df['walk_type'] == 'random_walk'
        ]
        metric_random_walk_pivot = pd.DataFrame()
        if not metric_random_walk_df.empty:
            metric_random_walk_pivot = metric_random_walk_df.pivot_table(index='r', columns='l', values=metric_name)
            # Ensure correct column order
            metric_random_walk_pivot = metric_random_walk_pivot[prefixed_l_values]

        metric_afi_df = combined_all_metrics_df[
            combined_all_metrics_df['walk_type'] == 'adaptative_first_improvement'
        ]
        metric_afi_pivot = pd.DataFrame()
        if not metric_afi_df.empty:
            metric_afi_pivot = metric_afi_df.pivot_table(index='r', values=metric_name)
            metric_afi_pivot.rename(columns={metric_name: 'adaptative_first_improvement'}, inplace=True)

        metric_and_df = combined_all_metrics_df[
            combined_all_metrics_df['walk_type'] == 'adaptative_non_dominated'
        ]
        metric_and_pivot = pd.DataFrame()
        if not metric_and_df.empty:
            metric_and_pivot = metric_and_df.pivot_table(index='r', values=metric_name)
            metric_and_pivot.rename(columns={metric_name: 'adaptative_non_dominated'}, inplace=True)

        # Concatenate the pivoted DataFrames horizontally
        dfs_to_concat = []
        if not metric_random_walk_pivot.empty:
            dfs_to_concat.append(metric_random_walk_pivot)
        if not metric_afi_pivot.empty:
            dfs_to_concat.append(metric_afi_pivot)
        if not metric_and_pivot.empty:
            dfs_to_concat.append(metric_and_pivot)

        if dfs_to_concat:
            combined_metric_df = pd.concat(dfs_to_concat, axis=1)
            # Reindex to ensure r_values order
            combined_metric_df = combined_metric_df.reindex(r_values)

            # Save to CSV
            metric_output_path = os.path.join(current_result_base_path, 'regression_metrics', f"{metric_name}_{LABEL}.csv")
            combined_metric_df.to_csv(metric_output_path)
            print(f"\n>>> {metric_name} table saved to: {metric_output_path}")
        else:
            print(f"\n>>> No metrics for {metric_name} found to save.")

    print(">>> Process Completed Successfully.")

# ==============================================================================
# EXECUTION
# ==============================================================================
if __name__ == "__main__":
    # Relative paths from this script's directory
    script_dir = os.path.dirname(os.path.abspath(__file__))
    data_folder_path = os.path.join(script_dir, '..', '..', 'Results', 'meta_features_with_indicators')
    base_output_path_for_experiments = os.path.join(script_dir, '..', '..', 'Results', 'meta_learning_results')
    experiment_label_HV = 'HV' # Define the label for the current experiment
    experiment_label_IGD = 'IGD'

    # We assume HV is a maximization metric and IGD is a minimization metric
    if os.path.exists(data_folder_path):
        run_pipeline(data_folder_path, experiment_label_HV, base_output_path_for_experiments, is_maximization=True)
        run_pipeline(data_folder_path, experiment_label_IGD, base_output_path_for_experiments, is_maximization=False)
    else:
        print(f"ERROR: Path not found: {data_folder_path}")
        print("Please, edit the variable 'data_folder_path' at the end of the script.")


